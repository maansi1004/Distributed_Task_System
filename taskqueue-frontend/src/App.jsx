import { useState, useEffect, useRef, useCallback } from 'react'
import { BarChart, Bar, XAxis, YAxis, Tooltip, ResponsiveContainer } from 'recharts'

const API = '/api'

const STATUS_COLORS = {
  PENDING: { bg: '#2d2a1a', text: '#f6c90e', dot: '#f6c90e' },
  RUNNING: { bg: '#1a2540', text: '#60a5fa', dot: '#60a5fa' },
  SUCCESS: { bg: '#0f2a1a', text: '#34d399', dot: '#34d399' },
  FAILED:  { bg: '#2a1010', text: '#f87171', dot: '#f87171' },
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
      <span style={{ width: 5, height: 5, borderRadius: '50%', background: s.dot }} />
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
      background: '#161b27',
      border: `0.5px solid ${busy ? '#1e3a5f' : '#2d3748'}`,
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

function ApiExplorer({ onRefresh }) {
  const [response, setResponse] = useState('')
  const [taskId, setTaskId] = useState('')
  const [description, setDescription] = useState('')
  const [priority, setPriority] = useState('1')
  const [loading, setLoading] = useState(false)
  const [activeBtn, setActiveBtn] = useState('')

  async function hit(method, path, body) {
    setLoading(true)
    setActiveBtn(method + path)
    try {
      const opts = { method }
      if (body) {
        opts.headers = { 'Content-Type': 'application/json' }
        opts.body = JSON.stringify(body)
      }
      const res = await fetch(API + path, opts)
      const text = await res.text()
      try {
        setResponse(JSON.stringify(JSON.parse(text), null, 2))
      } catch {
        setResponse(text)
      }
      onRefresh()
    } catch(e) {
      setResponse('Error: ' + e.message)
    } finally {
      setLoading(false)
    }
  }

  const btn = (label, method, path, body, color) => (
    <button
      onClick={() => hit(method, path, body)}
      style={{
        background: activeBtn === method + path ? color + '33' : '#161b27',
        border: `0.5px solid ${color}55`,
        color, borderRadius: 6, padding: '6px 14px',
        fontSize: 12, cursor: 'pointer', fontWeight: 500,
        transition: 'all 0.15s'
      }}
    >
      {label}
    </button>
  )

  return (
    <div style={{ background: '#161b27', border: '0.5px solid #2d3748', borderRadius: 10, padding: '16px', marginBottom: '1.5rem' }}>
      <div style={{ fontSize: 11, color: '#475569', textTransform: 'uppercase', letterSpacing: '0.06em', marginBottom: 14 }}>
        api explorer
      </div>

      <div style={{ display: 'flex', flexWrap: 'wrap', gap: 10, marginBottom: 14, alignItems: 'center' }}>
        <div style={{ display: 'flex', gap: 6, alignItems: 'center' }}>
          <input
            value={description}
            onChange={e => setDescription(e.target.value)}
            placeholder="description"
            style={inputStyle(160)}
          />
          <input
            value={priority}
            onChange={e => setPriority(e.target.value)}
            placeholder="priority"
            style={inputStyle(70)}
          />
          {btn('POST /tasks', 'POST', '/tasks', { description: description || 'API Task', priority: parseInt(priority) || 1 }, '#60a5fa')}
        </div>

        <div style={{ width: '0.5px', height: 24, background: '#2d3748' }} />

        {btn('GET /tasks', 'GET', '/tasks', null, '#34d399')}
        {btn('GET /metrics', 'GET', '/metrics', null, '#34d399')}

        <div style={{ width: '0.5px', height: 24, background: '#2d3748' }} />

        <div style={{ display: 'flex', gap: 6, alignItems: 'center' }}>
          <input
            value={taskId}
            onChange={e => setTaskId(e.target.value)}
            placeholder="task id"
            style={inputStyle(90)}
          />
          {btn('GET /tasks/:id', 'GET', `/tasks/${taskId}`, null, '#34d399')}
          {btn('DELETE /tasks/:id', 'DELETE', `/tasks/${taskId}`, null, '#f87171')}
        </div>
      </div>

      {loading && (
        <div style={{ fontSize: 12, color: '#475569', marginBottom: 8 }}>fetching…</div>
      )}

      {response && (
        <div style={{ position: 'relative' }}>
          <button
            onClick={() => setResponse('')}
            style={{ position: 'absolute', top: 8, right: 8, background: 'transparent', border: 'none', color: '#475569', cursor: 'pointer', fontSize: 12 }}
          >
            clear
          </button>
          <pre style={{
            background: '#0f1117', border: '0.5px solid #2d3748',
            borderRadius: 8, padding: '12px 40px 12px 12px',
            fontSize: 11, color: '#94a3b8',
            overflow: 'auto', maxHeight: 220,
            fontFamily: 'monospace', margin: 0,
            lineHeight: 1.6
          }}>
            {response}
          </pre>
        </div>
      )}
    </div>
  )
}

function inputStyle(width) {
  return {
    background: '#0f1117', border: '0.5px solid #2d3748',
    borderRadius: 6, padding: '6px 10px',
    color: '#e2e8f0', fontSize: 12, width
  }
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
    <div style={{ maxWidth: 980, margin: '0 auto', padding: '2rem 1.5rem' }}>

      {/* Header */}
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
            <span style={{ fontSize: 11, color: '#374151' }}>{lastUpdated.toLocaleTimeString()}</span>
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

      {/* Metrics */}
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(6, 1fr)', gap: 8, marginBottom: '1.5rem' }}>
        <MetricCard label="pending"     value={metrics?.pending}     color="#f6c90e" />
        <MetricCard label="running"     value={metrics?.running}     color="#60a5fa" />
        <MetricCard label="success"     value={metrics?.success}     color="#34d399" />
        <MetricCard label="failed"      value={metrics?.failed}      color="#f87171" />
        <MetricCard label="queue depth" value={metrics?.queue_depth} color="#e2e8f0" />
        <MetricCard label="dlq"         value={metrics?.dlq_depth}   color="#f87171" />
      </div>

      {/* Workers */}
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

      {/* Throughput chart */}
      <div style={{ background: '#161b27', border: '0.5px solid #2d3748', borderRadius: 10, padding: '14px 16px', marginBottom: '1.5rem' }}>
        <div style={{ fontSize: 11, color: '#475569', textTransform: 'uppercase', letterSpacing: '0.06em', marginBottom: 12 }}>throughput</div>
        <ResponsiveContainer width="100%" height={140}>
          <BarChart data={history} barSize={14}>
            <XAxis dataKey="label" tick={{ fontSize: 10, fill: '#374151' }} axisLine={false} tickLine={false} />
            <YAxis tick={{ fontSize: 10, fill: '#374151' }} axisLine={false} tickLine={false} allowDecimals={false} />
            <Tooltip content={<CustomTooltip />} cursor={{ fill: 'rgba(96,165,250,0.05)' }} />
            <Bar dataKey="completed" fill="#1d4ed8" radius={[3, 3, 0, 0]} />
          </BarChart>
        </ResponsiveContainer>
      </div>

      {/* API Explorer */}
      <ApiExplorer onRefresh={fetchAll} />

      {/* Tasks table */}
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
              <tr><td colSpan={5} style={{ padding: '2rem', textAlign: 'center', color: '#374151', fontSize: 13 }}>no tasks yet</td></tr>
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
