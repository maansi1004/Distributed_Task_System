import { useState, useEffect, useRef, useCallback } from 'react'
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, BarChart, Bar } from 'recharts'

const API = '/api'

const STATUS_COLORS = {
  PENDING:  { bg: '#2d2a1a', text: '#f6c90e', dot: '#f6c90e' },
  RUNNING:  { bg: '#1a2540', text: '#60a5fa', dot: '#60a5fa' },
  SUCCESS:  { bg: '#0f2a1a', text: '#34d399', dot: '#34d399' },
  FAILED:   { bg: '#2a1010', text: '#f87171', dot: '#f87171' },
}

function StatusPill({ status }) {
  const s = STATUS_COLORS[status] || STATUS_COLORS.FAILED
  return (
    <span style={{
      background: s.bg, color: s.text,
      fontSize: 11, fontWeight: 600,
      padding: '2px 10px', borderRadius: 99,
      letterSpacing: '0.04em', textTransform: 'uppercase',
      display: 'inline-flex', alignItems: 'center', gap: 5
    }}>
      <span style={{ width: 5, height: 5, borderRadius: '50%', background: s.dot, display: 'inline-block' }} />
      {status.toLowerCase()}
    </span>
  )
}

function MetricCard({ label, value, color }) {
  return (
    <div style={{
      background: '#161b27', border: '0.5px solid #2d3748',
      borderRadius: 10, padding: '14px 16px',
    }}>
      <div style={{ fontSize: 11, color: '#64748b', textTransform: 'uppercase', letterSpacing: '0.06em', marginBottom: 6 }}>
        {label}
      </div>
      <div style={{ fontSize: 28, fontWeight: 600, color: color || '#e2e8f0', fontVariantNumeric: 'tabular-nums' }}>
        {value ?? '–'}
      </div>
    </div>
  )
}

function WorkerCard({ name, task, busy, done }) {
  return (
    <div style={{
      background: '#161b27', border: `0.5px solid ${busy ? '#1e3a5f' : '#2d3748'}`,
      borderRadius: 10, padding: '12px 14px',
      transition: 'border-color 0.3s'
    }}>
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 8 }}>
        <span style={{ fontSize: 13, fontWeight: 500, fontFamily: 'monospace', color: '#e2e8f0' }}>{name}</span>
        <span style={{
          fontSize: 10, fontWeight: 600, padding: '2px 8px', borderRadius: 99,
          background: busy ? '#1a2540' : '#1a1a2a',
          color: busy ? '#60a5fa' : '#475569',
          textTransform: 'uppercase', letterSpacing: '0.05em'
        }}>
          {busy ? 'busy' : 'idle'}
        </span>
      </div>
      <div style={{ fontSize: 12, color: busy ? '#94a3b8' : '#475569', marginBottom: 4, fontFamily: 'monospace' }}>
        {busy ? `task #${task}` : 'waiting for task…'}
      </div>
      <div style={{ fontSize: 11, color: '#374151' }}>{done} tasks done</div>
    </div>
  )
}

const CustomTooltip = ({ active, payload, label }) => {
  if (!active || !payload?.length) return null
  return (
    <div style={{ background: '#1e2535', border: '0.5px solid #2d3748', borderRadius: 8, padding: '8px 12px', fontSize: 12 }}>
      <div style={{ color: '#64748b', marginBottom: 2 }}>{label}</div>
      <div style={{ color: '#60a5fa', fontWeight: 600 }}>{payload[0].value} completed</div>
    </div>
  )
}

export default function App() {
  const [metrics, setMetrics] = useState(null)
  const [tasks, setTasks] = useState([])
  const [history, setHistory] = useState([])
  const [lastUpdated, setLastUpdated] = useState(null)
  const [error, setError] = useState(null)
  const [submitting, setSubmitting] = useState(false)
  const prevSuccessRef = useRef(null)
  const workerDoneRef = useRef([0, 0, 0])

  const fetchAll = useCallback(async () => {
    try {
      const [mRes, tRes] = await Promise.all([
        fetch(API + '/metrics'),
        fetch(API + '/tasks')
      ])
      if (!mRes.ok || !tRes.ok) throw new Error('fetch failed')
      const [m, t] = await Promise.all([mRes.json(), tRes.json()])

      setMetrics(m)
      setTasks(t)
      setError(null)
      setLastUpdated(new Date())

      const now = new Date()
      const label = now.toLocaleTimeString('en', { hour12: false })
      const delta = prevSuccessRef.current !== null
        ? Math.max(0, (m.success ?? 0) - prevSuccessRef.current)
        : 0
      prevSuccessRef.current = m.success ?? 0

      setHistory(prev => {
        const next = [...prev, { label, completed: delta }]
        return next.slice(-15)
      })

      const running = t.filter(x => x.status === 'RUNNING')
      running.forEach((_, i) => {
        if (i < 3) workerDoneRef.current[i]++
      })
    } catch (e) {
      setError('Cannot reach API — is Docker running?')
    }
  }, [])

  useEffect(() => {
    fetchAll()
    const id = setInterval(fetchAll, 3000)
    return () => clearInterval(id)
  }, [fetchAll])

  async function submitTask() {
    setSubmitting(true)
    try {
      await fetch(API + '/tasks', { method: 'POST' })
      setTimeout(fetchAll, 400)
    } catch(e) {
      setError('Submit failed')
    } finally {
      setSubmitting(false)
    }
  }

  const runningTasks = tasks.filter(t => t.status === 'RUNNING')
  const recentTasks = [...tasks].reverse().slice(0, 12)

  return (
    <div style={{ maxWidth: 960, margin: '0 auto', padding: '2rem 1.5rem' }}>

      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '2rem' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 12 }}>
          <div style={{ width: 36, height: 36, background: '#1a2540', borderRadius: 8, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="#60a5fa" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
              <rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8M12 17v4"/>
            </svg>
          </div>
          <div>
            <div style={{ fontSize: 16, fontWeight: 600, color: '#e2e8f0' }}>Task Queue Monitor</div>
            <div style={{ fontSize: 12, color: '#475569' }}>distributed worker dashboard</div>
          </div>
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
          {!error && (
            <span style={{ display: 'flex', alignItems: 'center', gap: 5, fontSize: 12, color: '#34d399' }}>
              <span style={{ width: 6, height: 6, borderRadius: '50%', background: '#34d399', display: 'inline-block', animation: 'pulse 1.5s infinite' }} />
              live
            </span>
          )}
          {error && <span style={{ fontSize: 12, color: '#f87171' }}>{error}</span>}
          {lastUpdated && !error && (
            <span style={{ fontSize: 11, color: '#374151' }}>
              {lastUpdated.toLocaleTimeString()}
            </span>
          )}
          <button onClick={submitTask} disabled={submitting} style={{
            background: '#1a2540', border: '0.5px solid #2d4a7a', color: '#60a5fa',
            borderRadius: 8, padding: '6px 14px', fontSize: 13, cursor: 'pointer',
            display: 'flex', alignItems: 'center', gap: 6
          }}>
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round"><path d="M12 5v14M5 12h14"/></svg>
            {submitting ? 'submitting…' : 'new task'}
          </button>
          <button onClick={fetchAll} style={{
            background: 'transparent', border: '0.5px solid #2d3748', color: '#64748b',
            borderRadius: 8, padding: '6px 10px', fontSize: 13, cursor: 'pointer'
          }}>
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round"><path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/><path d="M3 3v5h5"/></svg>
          </button>
        </div>
      </div>

      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(6, 1fr)', gap: 8, marginBottom: '1.5rem' }}>
        <MetricCard label="pending"     value={metrics?.pending}        color="#f6c90e" />
        <MetricCard label="running"     value={metrics?.running}        color="#60a5fa" />
        <MetricCard label="success"     value={metrics?.success}        color="#34d399" />
        <MetricCard label="failed"      value={metrics?.failed}         color="#f87171" />
        <MetricCard label="queue depth" value={metrics?.queue_depth}    color="#e2e8f0" />
        <MetricCard label="dlq"         value={metrics?.dlq_depth}      color="#f87171" />
      </div>

      <div style={{ marginBottom: '1.5rem' }}>
        <div style={{ fontSize: 11, color: '#475569', textTransform: 'uppercase', letterSpacing: '0.06em', marginBottom: 10 }}>workers</div>
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(3, 1fr)', gap: 8 }}>
          {[0, 1, 2].map(i => (
            <WorkerCard
              key={i}
              name={`worker-${i + 1}`}
              busy={!!runningTasks[i]}
              task={runningTasks[i]?.id}
              done={workerDoneRef.current[i]}
            />
          ))}
        </div>
      </div>

      <div style={{ background: '#161b27', border: '0.5px solid #2d3748', borderRadius: 10, padding: '14px 16px', marginBottom: '1.5rem' }}>
        <div style={{ fontSize: 11, color: '#475569', textTransform: 'uppercase', letterSpacing: '0.06em', marginBottom: 12 }}>throughput</div>
        <ResponsiveContainer width="100%" height={160}>
          <BarChart data={history} barSize={16}>
            <XAxis dataKey="label" tick={{ fontSize: 10, fill: '#374151' }} axisLine={false} tickLine={false} />
            <YAxis tick={{ fontSize: 10, fill: '#374151' }} axisLine={false} tickLine={false} allowDecimals={false} />
            <Tooltip content={<CustomTooltip />} cursor={{ fill: 'rgba(96,165,250,0.05)' }} />
            <Bar dataKey="completed" fill="#1d4ed8" radius={[3, 3, 0, 0]} />
          </BarChart>
        </ResponsiveContainer>
      </div>

      <div style={{ background: '#161b27', border: '0.5px solid #2d3748', borderRadius: 10, overflow: 'hidden' }}>
        <div style={{ padding: '12px 16px', borderBottom: '0.5px solid #2d3748', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <span style={{ fontSize: 11, color: '#475569', textTransform: 'uppercase', letterSpacing: '0.06em' }}>recent tasks</span>
          <span style={{ fontSize: 11, color: '#374151' }}>{tasks.length} total</span>
        </div>
        <table style={{ width: '100%', borderCollapse: 'collapse', fontSize: 13 }}>
          <thead>
            <tr style={{ borderBottom: '0.5px solid #2d3748' }}>
              {['id', 'description', 'status', 'priority', 'retries'].map(h => (
                <th key={h} style={{ textAlign: 'left', padding: '8px 16px', fontSize: 11, color: '#475569', fontWeight: 500, textTransform: 'uppercase', letterSpacing: '0.04em' }}>{h}</th>
              ))}
            </tr>
          </thead>
          <tbody>
            {recentTasks.length === 0 ? (
              <tr><td colSpan={5} style={{ padding: '2rem', textAlign: 'center', color: '#374151', fontSize: 13 }}>no tasks yet — click "new task" to submit one</td></tr>
            ) : recentTasks.map(t => (
              <tr key={t.id} style={{ borderBottom: '0.5px solid #1e2535' }}>
                <td style={{ padding: '10px 16px', fontFamily: 'monospace', fontSize: 12, color: '#475569' }}>#{t.id}</td>
                <td style={{ padding: '10px 16px', color: '#94a3b8', maxWidth: 200, overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>{t.description || '—'}</td>
                <td style={{ padding: '10px 16px' }}><StatusPill status={t.status} /></td>
                <td style={{ padding: '10px 16px', color: '#475569' }}>{t.priority}</td>
                <td style={{ padding: '10px 16px', color: '#475569' }}>{t.retry_count ?? 0}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      <style>{`@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }`}</style>
    </div>
  )
}
